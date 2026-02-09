create or replace function immutable_unaccent(input text)
returns text
language sql
immutable
strict
parallel safe
as $$
  select unaccent('unaccent'::regdictionary, input);
$$;

create index if not exists stash_search_trgm_idx
on stash using gin (
  (
    lower(
      immutable_unaccent(
        coalesce(physical_label, '') || ' ' ||
        coalesce(category, '') || ' ' ||
        coalesce(sub_category, '') || ' ' ||
        coalesce(contents, '')
      )
    )
  ) gin_trgm_ops
);
