alter table stash
  add column if not exists created_at timestamptz not null default now(),
  add column if not exists updated_at timestamptz not null default now();

create or replace function set_updated_at()
returns trigger
language plpgsql
as $$
begin
  new.updated_at = now();
  return new;
end;
$$;

drop trigger if exists trg_stash_set_updated_at on stash;

create trigger trg_stash_set_updated_at
before update on stash
for each row
execute function set_updated_at();
